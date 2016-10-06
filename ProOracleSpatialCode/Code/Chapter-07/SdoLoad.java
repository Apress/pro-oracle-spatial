/*
 * @(#)SdoLoad.java 1.0 12-Jul-2003
 *
 * This program reads a geometry from an ascii text file, and inserts
 * it in a database table.
 *
 * It uses the Java API for Oracle Spatial supplied with
 * version 10.1 or the Oracle Server (class JGeometry)
 *
 * The program lets you specify connection parameters to a database, as
 * well as the name of the table and the name of the geometry column to
 * load into. It also lets you specify the name of an identification
 * column (a number) to identify the row to insert or update.
 *
 * The input file is structured as described in the Region class
 *
 */

import java.io.*;
import java.sql.*;
import java.util.*;
import java.awt.geom.*;
import oracle.jdbc.driver.*;
import oracle.sql.STRUCT;
import oracle.spatial.geometry.JGeometry;

public final class SdoLoad
{
  public static void main(String args[]) throws Exception
  {
    System.out.println ("SdoLoad - Oracle Spatial (SDO) load example");

    // Check and process command line arguments
    if (args.length != 9) {
      System.out.println ("Parameters:");
      System.out.println ("<Connection>:  JDBC connection string");
      System.out.println ("               e.g: jdbc:oracle:thin:@server:port:sid");
      System.out.println ("<User>:        User name");
      System.out.println ("<Password>:    User password");
      System.out.println ("<Table name>:  Table to import");
      System.out.println ("<Geo column>:  Name of geometry colum");
      System.out.println ("<ID column>:   Name of geometry id colum");
      System.out.println ("<ID value>:    Value of geometry ID");
      System.out.println ("<Input File>:  Name of input file");
      System.out.println ("<Action>:      I for insert or U for pdate");
      return;
    }
    String  connectionString  = args[0];
    String  userName          = args[1];
    String  userPassword      = args[2];
    String  tableName         = args[3];
    String  geoColumn         = args[4];
    String  idColumn          = args[5];
    int     idValue           = Integer.parseInt(args[6]);
    String  inputFileName     = args[7];
    boolean insertAction      = (args[8].compareTo("I")==0 ? true : false);

    // Open input file
    System.out.println ("Opening file '" + inputFileName + "'");
    BufferedReader inputFile =
       new BufferedReader(new FileReader(inputFileName));

    // Register the Oracle JDBC driver
    DriverManager.registerDriver(new oracle.jdbc.driver.OracleDriver());

    // Get a connection to the database
    System.out.println ("Connecting to database '"+connectionString+"'");
    Connection dbConnection = DriverManager.getConnection(connectionString, userName, userPassword);

    // Load the geometry from the file
    loadGeometry(dbConnection, tableName, geoColumn, idColumn, idValue, inputFile, insertAction);

    // Close database connection
    dbConnection.close();

    // Close input file
    inputFile.close();
  }

  static void loadGeometry(Connection dbConnection, String tableName,
    String geoColumn, String idColumn, int idValue, BufferedReader inputFile,
    boolean insertAction)
    throws SQLException, IOException
  {
    // Construct the SQL statement
    String SqlStatement;
    if (insertAction)
      SqlStatement = "INSERT INTO " + tableName + " (" + geoColumn + "," + idColumn + ") " +
        "VALUES (?, ?)";
    else
      SqlStatement = "UPDATE " + tableName + " SET " + geoColumn + " = ? "
        + "WHERE " + idColumn + " = ?";
    System.out.println ("Executing query: '"+SqlStatement+"'");

    // Prepare the SQL statement
    PreparedStatement stmt = dbConnection.prepareStatement(SqlStatement);

    // Load geomety Region from input file
    Region polygon = new Region(inputFile);
    System.out.println ("Region geometry loaded ("
      + polygon.getNrOfPoints() + " points)");

    // Get the list of points of the region
    double ordinates[] = polygon.getOrdinates();

    // Construct new JGeometry object
    JGeometry geom = JGeometry.createLinearPolygon(ordinates, 2, 8307);

    // Convert object into java STRUCT
    STRUCT dbObject = JGeometry.store (geom, dbConnection);

    // Insert or update row in the database table
    stmt.setObject (1,dbObject);
    stmt.setInt (2, idValue);
    stmt.execute();

    stmt.close();
  }


  /* Class Region

     This class defines the geometry of a region as obtained from
     an ascii file.

     The input is a list of records:

       NUM_POINTS 26
       POINT 1 -77.120201 38.9342
       POINT 2 -77.101501 38.910999
       ......
       POINT 26 -77.120201 38.9342

     The first line defines the number of points that follow.
     Then each point is defined on a separate line.

     The program performs minimal parsing and validation of the
     input file.
  */
  static class Region {

    int nrOfPoints = 0;
    Point2D points[] = null;

    public Region(BufferedReader inputFile)
      throws IOException {
      this.loadFromFile (inputFile);
    }

    public void loadFromFile (BufferedReader inputFile)
      throws IOException {

      String s;
      while((s = inputFile.readLine())!= null) {
        StringTokenizer st = new StringTokenizer (s);
        int n = st.countTokens();
        if (n > 0) {
          String tk = st.nextToken();
          if (tk.compareTo ("NUM_POINTS") == 0) {
            nrOfPoints = Integer.parseInt(st.nextToken());
            points = new Point2D[nrOfPoints];
          }
          if (tk.compareTo ("POINT") == 0) {
            int pn = Integer.parseInt(st.nextToken());
            double x = Double.parseDouble(st.nextToken());
            double y = Double.parseDouble(st.nextToken());
            points[pn-1] = new Point2D.Double(x,y);
          }
        }
      }
    }
    public Point2D[] getPoints() {
      return points;
    }
    public double[] getOrdinates() {
      double ordinates[] = new double[nrOfPoints * 2];
      for (int i=0; i<points.length; i++) {
        ordinates [i*2] = points[i].getX();
        ordinates [i*2+1] = points[i].getY();
      }
      return ordinates;
    }
    public int getNrOfPoints() {
      return nrOfPoints;
    }

  }
}
